import React from "react";

  type ButtonProps = {
    text: string;
    onClick: () => void;
  };

  const Button: React.FC<ButtonProps> = ({text, onClick}) => {
    return (
      <button className="button" onClick={onClick}>
        <span>{text}</span>
      </button>
    );
  }

  export default Button;

