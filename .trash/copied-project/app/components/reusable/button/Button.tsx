import React from "react";

  type PrimaryButtonProps = {
    text: string;
    onClick: () => void;
  };

  const PrimaryButton: React.FC<PrimaryButtonProps> = ({text, onClick}) => {
    return (
      <button className="button" onClick={onClick}>
        <span>{text}</span>
      </button>
    );
  }

  export default PrimaryButton;

